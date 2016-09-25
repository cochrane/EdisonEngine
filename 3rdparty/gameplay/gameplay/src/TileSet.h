#pragma once

#include "Drawable.h"
#include "SpriteBatch.h"


namespace gameplay
{
    /**
     * Defines a grid of tiles for rendering a 2D planer region.
     *
     * The UVs are sourced from the texture atlas image.
     * The columns(x) and rows(y) are set individually for each
     * in the tile set. Specifying a source (x,y) of -1, -1 will allow
     * for tiles in the set to be empty.
     * The size of all the tiles must be equal to the tileWidth/tileHeight
     * provided.
     *
     * To avoid seams in the tiles the tile should be padded with
     * a gutter of duplicate pixels on each side of the region.
     *
     * The tile set does not support rotation or scaling.
     */
    class TileSet : public Drawable
    {
        friend class Node;

    public:

        /**
         * Sets the tile source location for the specified column and row.
         *
         * @param column The column to set the source for.
         * @param row The row to set the source for.
         * @param source The source top-left corner where the tile is positioned.
         */
        void setTileSource(unsigned int column, unsigned int row, const glm::vec2& source);

        /**
         * Gets the source clip region and flip flags for the specified column and row.
         *
         * @param column The column to get the source clip region and flip flags for.
         * @param row The row to specify the source clip region and flip flags for.
         * @param source The source region to be returned back.
         * @see Sprite::FlipFlags
         */
        void getTileSource(unsigned int column, unsigned int row, glm::vec2* source);

        /**
         * Gets the width of each tile in the tile set.
         *
         * @return The width of each tile in the tile set.
         */
        float getTileWidth() const;

        /**
         * Gets the height of each tile in the tile set.
         *
         * @return The height of each tile in the tile set.
         */
        float getTileHeight() const;

        /**
         * Gets the number of tile rows.
         *
         * @return The number of tile rows.
         */
        unsigned int getRowCount() const;

        /**
         * Gets the number of tile columns.
         *
         * @return The number of tile columns.
         */
        unsigned int getColumnCount() const;

        /**
         * Gets the overall width of the tileset.
         *
         * @return The overall width of the tileset.
         */
        float getWidth() const;

        /**
         * Gets the overall width of the tileset.
         *
         * @return The overall width of the tileset.
         */
        float getHeight() const;

        /**
         * Sets the opacity for the sprite.
         *
         * The range is from full transparent to opaque [0.0,1.0].
         *
         * @param opacity The opacity for the sprite.
         */
        void setOpacity(float opacity);

        /**
         * Gets the opacity for the sprite.
         *
         * The range is from full transparent to opaque [0.0,1.0].
         *
         * @return The opacity for the sprite.
         */
        float getOpacity() const;

        /**
         * Sets the color (RGBA) for the sprite.
         *
         * @param color The color(RGBA) for the sprite.
         */
        void setColor(const glm::vec4& color);

        /**
         * Gets the color (RGBA) for the sprite.
         *
         * @return The color(RGBA) for the sprite.
         */
        const glm::vec4& getColor() const;

        /**
         * @see Drawable::draw
         */
        size_t draw(bool wireframe = false) override;

    protected:
        /**
         * Constructor
         */
        TileSet();

        /**
         * Destructor
         */
        ~TileSet();

        /**
         * operator=
         */
        TileSet& operator=(const TileSet& set) = delete;

    private:

        glm::vec2* _tiles;
        float _tileWidth;
        float _tileHeight;
        unsigned int _rowCount;
        unsigned int _columnCount;
        float _width;
        float _height;
        SpriteBatch* _batch;
        float _opacity;
        glm::vec4 _color;
    };
}
